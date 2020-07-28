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
 *   MtkOmxAdpcmEnc.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX ADPCM Encoder component
 *
 * Author:
 * -------
 *   We Li(mtk81277)
 *
 ****************************************************************************/
 #define MTK_LOG_ENABLE 1

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <cutils/log.h>
 #include <cutils/properties.h>
 #include <dlfcn.h>
 #include "osal_utils.h"

 #include <time.h>

 #include "MtkOmxAdpcmEnc.h"

 #define LOG_TAG "MtkOmxAdpcmEnc"



 MtkOmxAdpcmEnc::MtkOmxAdpcmEnc()
 : mpHandle(NULL)
 , mEncoderInitFlag(OMX_FALSE)
 , mBitStreamBufferForEncoderSize(0)
 , mInternalWorkingBufferSize(0)
 , mpInternalWorkingBuffer(NULL)
 , mExtraDataSize(0)
 , mpExtraData(NULL)
 , mSamplesPerBlock(0)
 , mBlockAlign(0)
 , PCMBufferSizeForOneBlock(0)
 , mStartTime(0LL)
 , mLastSamplesCosumed(0)
 , maxNumFramesInOutputBuf(1)
 , isNewOutputBuffer(OMX_TRUE)
 , numFramesInOutputBuf(0)
 , totalConsumedBytes(0)
 , mDumpProp("vendor.audio.dumpenc.adpcm")
 {
	SLOGD("ADPCM Encoder Constructor +++");
	memset(&ADPCMPendingBuffer, 0, sizeof(ADPCMPendingBuffer));
	memset(&mOutputAdpcmParam, 0, sizeof(mOutputAdpcmParam));
 }

 MtkOmxAdpcmEnc :: ~MtkOmxAdpcmEnc()
 {
 	SLOGD("ADPCM Encoder Deconstructor +++");
	if(OMX_TRUE == mEncoderInitFlag)
	{
		DeinitAudioEncoder();
	}
 }

 void MtkOmxAdpcmEnc :: EncodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf)
 {
    if (NULL == pInputBuf ||NULL == pOutputBuf)
 	{
		SLOGE("pInputBuf == NULL or pOutputBuf == NULL !");
		handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);

		OMX_ERRORTYPE err = OMX_ErrorBadParameter;
		mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);

		return ;
	}
 	SLOGD("Begin EncodeAudio, pInputBuf->nFilledLength is %u, pOutputBuf->nFilledLength is %d", pInputBuf->nFilledLen, pOutputBuf->nFilledLen);
	SLOGD("Begin EncodeAudio, pInputBuf->nTimeStamp is %lld, pOutputBuf->nTimeStamp is %lld", pInputBuf->nTimeStamp, pOutputBuf->nTimeStamp);

	if(OMX_FALSE == mEncoderInitFlag)
	{
		if(OMX_TRUE == InitAudioEncoder(pInputBuf))
		{
			mEncoderInitFlag = OMX_TRUE;
			SLOGD("Init ADPCM Encoder Success, Port Settings Changed");
			mPortReconfigInProgress = OMX_TRUE;
			mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventPortSettingsChanged, MTK_OMX_OUTPUT_PORT, NULL, NULL);

			handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
		}
		else
		{
			SLOGE("Init Audio Encoder Failed !!!");
			mEncoderInitFlag = OMX_FALSE;

			FlushAudioEncoder();
			OMX_ERRORTYPE err = OMX_ErrorBadParameter;
			mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);

			handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
		}

		isNewOutputBuffer = OMX_TRUE;
	}
	else
	{
		if((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0))
		{
			SLOGD("ADPCM Encoder Component Receive EOS Without Data, TS=%lld !!!", pInputBuf->nTimeStamp);

			pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
			pOutputBuf->nTimeStamp = mStartTime + (mLastSamplesCosumed + mLastSamplesCosumed >> 1) * 1000000LL / (mOutputAdpcmParam.nSamplesPerSec);
			FlushAudioEncoder();

			handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
			SLOGD("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);

			return ;
		}

		if(isNewOutputBuffer == OMX_TRUE)
		{
			ALOGD("isNewOutputBuffer == OMX_TRUE !");

			if((mStartTime == 0) && (mLastSamplesCosumed == 0))
			{
				mStartTime =  pInputBuf->nTimeStamp ;
				pOutputBuf->nTimeStamp = mStartTime;
			}
			else
			{
				if(mOutputAdpcmParam.nSamplesPerSec)
				{
					pOutputBuf->nTimeStamp = mStartTime + (mLastSamplesCosumed + mLastSamplesCosumed >> 1) * 1000000LL / mOutputAdpcmParam.nSamplesPerSec;
					mStartTime = pOutputBuf->nTimeStamp;
					mLastSamplesCosumed = 0;
				}
			}
		}

		// encode frame
	    OMX_U8* pPcmBuffer = pInputBuf->pBuffer + pInputBuf->nOffset + totalConsumedBytes;
	    OMX_U32 nPcmBufSize = pInputBuf->nFilledLen;
	    OMX_U8* pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset + pOutputBuf->nFilledLen;
	    OMX_U32 nBitstreamBufSize = pOutputBuf->nAllocLen - pOutputBuf->nFilledLen - pOutputBuf->nOffset;  //store bitstream
	    OMX_U32 ulOutputSize = 0;
	    OMX_U32 ulConsumeSize = 0;

		if(ADPCMPendingBuffer.dataLength > 0)
		{
			SLOGD("ADPCMPendingBuffer.dataLength > 0 is %d, ADPCMPendingBuffer.bufferSize is %d", ADPCMPendingBuffer.dataLength, ADPCMPendingBuffer.bufferSize);
			if(ADPCMPendingBuffer.dataLength < ADPCMPendingBuffer.bufferSize)
			{
				OMX_U32 copy_to_pending_buffer_size = ADPCMPendingBuffer.bufferSize - ADPCMPendingBuffer.dataLength;
				if(copy_to_pending_buffer_size > nPcmBufSize)
				{
					copy_to_pending_buffer_size = nPcmBufSize;
				}
				memcpy((void *)((OMX_U8 *)ADPCMPendingBuffer.pBlockPCMBuffer + ADPCMPendingBuffer.dataLength), pPcmBuffer, copy_to_pending_buffer_size);
				ADPCMPendingBuffer.dataLength += copy_to_pending_buffer_size;

				pPcmBuffer += copy_to_pending_buffer_size;
				nPcmBufSize -= copy_to_pending_buffer_size;
				pInputBuf->nFilledLen -= copy_to_pending_buffer_size;  // input related just

				totalConsumedBytes += copy_to_pending_buffer_size;     //comsume data from input buffer
			}

			if((ADPCMPendingBuffer.dataLength >= ADPCMPendingBuffer.bufferSize) && (nBitstreamBufSize >= mBitStreamBufferForEncoderSize)
				&& (numFramesInOutputBuf < maxNumFramesInOutputBuf))
			{
				SLOGD("ADPCMPendingBuffer.dataLength is %d, nBitstreamBufSize is %d",ADPCMPendingBuffer.dataLength, nBitstreamBufSize);
				ulOutputSize = nBitstreamBufSize;
				ulConsumeSize = ADPCM_Encode(mpHandle, (unsigned int *)ADPCMPendingBuffer.pBlockPCMBuffer, ADPCMPendingBuffer.bufferSize, pBitStreamBuf, (unsigned int *)&ulOutputSize);

				pBitStreamBuf += ulOutputSize;
				nBitstreamBufSize -= ulOutputSize;

				pOutputBuf->nFilledLen += ulOutputSize;  //write data to output buffer just
				mLastSamplesCosumed += (ulConsumeSize / mOutputAdpcmParam.nChannelCount) >> 1;
				numFramesInOutputBuf++;

				ADPCMPendingBuffer.dataLength -= ulConsumeSize;

				if(mdumpflag == 1)
					Dump_PCMData(mDumpFile, (OMX_U8 *)ADPCMPendingBuffer.pBlockPCMBuffer, ADPCMPendingBuffer.bufferSize - ADPCMPendingBuffer.dataLength);
			}
		}

		while((nPcmBufSize >= ADPCMPendingBuffer.bufferSize) && (nBitstreamBufSize >= mBitStreamBufferForEncoderSize)
			&& (numFramesInOutputBuf < maxNumFramesInOutputBuf))
		{
			SLOGD("nPcmBufSize is %d, nBitstreamBufSize is %d",nPcmBufSize, nBitstreamBufSize);
			if(mdumpflag == 1)
				Dump_PCMData(mDumpFile, (OMX_U8 *)pPcmBuffer, ADPCMPendingBuffer.bufferSize);

			ulOutputSize = nBitstreamBufSize;
			ulConsumeSize = ADPCM_Encode(mpHandle, pPcmBuffer, ADPCMPendingBuffer.bufferSize, pBitStreamBuf, (unsigned int *)&ulOutputSize);

			pPcmBuffer += ulConsumeSize;
			nPcmBufSize -= ulConsumeSize;
			pBitStreamBuf += ulOutputSize;
			nBitstreamBufSize -= ulOutputSize;

			pInputBuf->nFilledLen -= ulConsumeSize;
			pOutputBuf->nFilledLen += ulOutputSize;
			mLastSamplesCosumed += (ulConsumeSize / mOutputAdpcmParam.nChannelCount) >> 1;

			totalConsumedBytes += ulConsumeSize;
			numFramesInOutputBuf++;
		}

		if((nBitstreamBufSize >= mBitStreamBufferForEncoderSize) && (numFramesInOutputBuf < maxNumFramesInOutputBuf))
		{
			SLOGD("PCM size is smaller than %d, Output buffer is available !", ADPCMPendingBuffer.bufferSize);
			if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
			{
				SLOGD("ADPCM EOS with data received, data has been consumed !");
				FlushAudioEncoder();
				pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;

				handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
			}
			else
			{
				SLOGD("nPcmBufSize is %d, pInputBuf->nFilledLen is %d", nPcmBufSize, pInputBuf->nFilledLen);
				SLOGD("ADPCMPendingBuffer.bufferSize is %d, ADPCMPendingBuffer.dataLength is %d", ADPCMPendingBuffer.bufferSize, ADPCMPendingBuffer.dataLength);
				if(nPcmBufSize > 0)
				{
					OMX_U32 copy_to_pending_buffer_size1 = 0;
					OMX_U32 remainSizeInPendingBuf = ADPCMPendingBuffer.bufferSize - ADPCMPendingBuffer.dataLength;
					if(nPcmBufSize <= remainSizeInPendingBuf)
					{
						copy_to_pending_buffer_size1 = nPcmBufSize;
					}
					else
						copy_to_pending_buffer_size1 = remainSizeInPendingBuf;

					memcpy(ADPCMPendingBuffer.pBlockPCMBuffer, pPcmBuffer, copy_to_pending_buffer_size1);
					nPcmBufSize -= copy_to_pending_buffer_size1;
					pInputBuf->nFilledLen -= copy_to_pending_buffer_size1;
					totalConsumedBytes += copy_to_pending_buffer_size1;

					ADPCMPendingBuffer.dataLength += copy_to_pending_buffer_size1;

					if(nPcmBufSize > 0)
					{
						SLOGD("After writing to pending buffer, input buffer still have data !");
						handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_FALSE);
					}
					else
					{
						SLOGD("After writing to pending buffer, input buffer do not have data !");
						handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_FALSE);
					}
				}
				else
				{
					SLOGD("PCM Data Size is Zero !");
					handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_FALSE);
				}
			}
		}
		else
		{
			if(nPcmBufSize >= ADPCMPendingBuffer.bufferSize)
			{
				SLOGD("Remaining data size is bigger than %d, output buffer is not available", ADPCMPendingBuffer.bufferSize);
				handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
			}
			else
			{
				SLOGD("Remaining data size is Smaller than %d, output buffer is not available", ADPCMPendingBuffer.bufferSize);
				if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
				{
					SLOGD("ADPCM Encoder EOS with data received !");
					FlushAudioEncoder();
					pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;

					handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
				}
				else
				{
					OMX_U32 copy_to_pending_buffer_size2 = 0;
					OMX_U32 remainSizeInPendingBuf = ADPCMPendingBuffer.bufferSize - ADPCMPendingBuffer.dataLength;
					if(nPcmBufSize <= remainSizeInPendingBuf)
					{
						copy_to_pending_buffer_size2 = nPcmBufSize;
					}
					else
						copy_to_pending_buffer_size2 = remainSizeInPendingBuf;

					memcpy(ADPCMPendingBuffer.pBlockPCMBuffer, pPcmBuffer, copy_to_pending_buffer_size2);
					nPcmBufSize -= copy_to_pending_buffer_size2;
					pInputBuf->nFilledLen -= copy_to_pending_buffer_size2;
					totalConsumedBytes += copy_to_pending_buffer_size2;

					ADPCMPendingBuffer.dataLength += copy_to_pending_buffer_size2;

					if(nPcmBufSize > 0)
					{
						SLOGD("Two After writing to pending buffer, input buffer still have data !");
						handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
					}
					else
					{
						SLOGD("Two After writing to pending buffer, input buffer do not have data !");
						handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
					}
				}
			}
		}
	}
}

 OMX_BOOL MtkOmxAdpcmEnc :: InitAudioEncoder(OMX_BUFFERHEADERTYPE* pInputBuf)
 {
 	SLOGD("Init Audio Encoder+++");
        OMX_BOOL err = OMX_TRUE;

	OMX_U32 versionID = 0;
	versionID = ADPCM_GetEncVersion();
	SLOGD("ADPCM Encoder Version is Ox%x", versionID);
	SLOGD("projectType is %d, compilerMajorVersion is %d, compileMinorVersion is %d, ReleaseVersion is %d",
		 (versionID & 0xFF000000)>>24,(versionID & 0x00FF0000)>>16,(versionID & 0x0000FF00)<<8,(versionID & 0x000000FF));

	ADPCM_ENC_PARAM encParam;
	encParam.enc_format = (mOutputAdpcmParam.nFormatTag == WAVE_FORMAT_MS_ADPCM) ? FORMAT_ENC_MS_ADPCM : FORMAT_ENC_DVI_IMAADPCM;
	encParam.channel_num = mOutputAdpcmParam.nChannelCount;
	encParam.sample_rate = mOutputAdpcmParam.nSamplesPerSec;
	mSamplesPerBlock = ADPCM_GetBlockSizeByDuration(MTK_DEFAULT_DURATION_FOR_ADPCM_BLOCK, &encParam);
	mBlockAlign = encParam.block_align_size;
	SLOGD("mSamplesPerBlock is %d, mBlockAlign is %d", mSamplesPerBlock, mBlockAlign);
	mOutputAdpcmParam.nBlockAlign = mBlockAlign;

	PCMBufferSizeForOneBlock = mSamplesPerBlock * mOutputAdpcmParam.nChannelCount * 2;
	ADPCM_ENC_STATUS encStatus = ADPCM_GetEncBufferSize(PCMBufferSizeForOneBlock, (unsigned int *)&mBitStreamBufferForEncoderSize, (unsigned int *)&mInternalWorkingBufferSize);
	if(ADPCM_ENC_SUCCESS != encStatus)
	{
		SLOGE("ADPCM GetEncBuffer failed !!!");
		err = OMX_FALSE;
		return err;
	}
	else
	{
		SLOGD("mBitStreamBufferForEncoderSize is %d, mInternalWorkingBufferSize is %d",mBitStreamBufferForEncoderSize, mInternalWorkingBufferSize);
	}
	mpInternalWorkingBuffer = (OMX_PTR)MTK_OMX_ALLOC(mInternalWorkingBufferSize);
	if(NULL == mpInternalWorkingBuffer)
	{
		SLOGE("Malloc Internal Working Buffer Failed !!!");
		err = OMX_FALSE;
		return err;
	}

	mpHandle = ADPCM_InitEncoder(mpInternalWorkingBuffer, &encParam);
	if(NULL == mpHandle)
	{
		SLOGE("Init ADPCM Encoder Failed !!!");
		err = OMX_FALSE;
		return err;
	}

	mExtraDataSize = ADPCM_GetEncExtraDataSize(mpHandle);
	mpExtraData = (OMX_PTR)MTK_OMX_ALLOC(mExtraDataSize);
	SLOGD("mExtraDataSize is %d, mpExtraData is 0x%x", mExtraDataSize, (unsigned long int)mpExtraData);
	if(NULL == mpExtraData)
	{
		SLOGE("Malloc Extra Data Buffer Failed !!!");
		err = OMX_FALSE;
		return err;
	}
	ADPCM_GetEncExtraData(mpHandle, mpExtraData, mExtraDataSize);

	//allocating ADPCM Pending Buffer
	ADPCMPendingBuffer.bufferSize = PCMBufferSizeForOneBlock;
	ADPCMPendingBuffer.dataLength = 0;
	ADPCMPendingBuffer.pBlockPCMBuffer = (OMX_U8 *)MTK_OMX_ALLOC(PCMBufferSizeForOneBlock);
	if(NULL == ADPCMPendingBuffer.pBlockPCMBuffer)
	{
		SLOGE("Malloc PCM Buffer For One Block Failed !!!");
		err = OMX_FALSE;
		return err;
	}

	getDumpFilePath();
	Dump_Access(mDumpProp, mDumpFile);

	err = OMX_TRUE;
	return err;
 }

 void MtkOmxAdpcmEnc :: FlushAudioEncoder()
 {
 	SLOGD("Flush Audio Encoder +++");

	if(OMX_TRUE == mEncoderInitFlag)
	{
		if(mpHandle)
		{
			ADPCM_FlushEncoder(mpHandle);
		}
		memset(ADPCMPendingBuffer.pBlockPCMBuffer, 0, ADPCMPendingBuffer.bufferSize);
		ADPCMPendingBuffer.dataLength = 0;
	}
	mStartTime = 0;
	mLastSamplesCosumed = 0;

	isNewOutputBuffer = OMX_TRUE;
	numFramesInOutputBuf = 0;
	totalConsumedBytes = 0;
 }

 void MtkOmxAdpcmEnc :: DeinitAudioEncoder()
 {
 	SLOGD("Deinit Audio Encoder+++");
	if(mpInternalWorkingBuffer)
	{
		MTK_OMX_FREE(mpInternalWorkingBuffer);
		mpInternalWorkingBuffer = NULL;
	}

	if(ADPCMPendingBuffer.pBlockPCMBuffer)
	{
		MTK_OMX_FREE(ADPCMPendingBuffer.pBlockPCMBuffer);
		ADPCMPendingBuffer.pBlockPCMBuffer = NULL;
	}

	if(mpExtraData)
	{
		MTK_OMX_FREE(mpExtraData);
		mpExtraData = NULL;
	}

	mpHandle = NULL;
	mEncoderInitFlag = OMX_FALSE;

	isNewOutputBuffer = OMX_TRUE;
	numFramesInOutputBuf = 0;
	totalConsumedBytes = 0;
 }

 OMX_ERRORTYPE MtkOmxAdpcmEnc :: GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
										 OMX_IN OMX_INDEXTYPE nParamIndex,
										 OMX_INOUT OMX_PTR ComponentParameterStructure)
 {
	OMX_ERRORTYPE err = OMX_ErrorNone;

	if(NULL == ComponentParameterStructure)
	{
		err = OMX_ErrorBadParameter;
		goto EXIT;
	}

	if(mState == OMX_StateInvalid)
	{
		err = OMX_ErrorIncorrectStateOperation;
		goto EXIT;
	}

	switch (nParamIndex)
	{
		case OMX_IndexParamAudioAdpcm:
		{
			SLOGD("GetParameter OMX_IndexParamAudioAdpcm !!!");
			OMX_AUDIO_PARAM_ADPCMTYPE *pParam = (OMX_AUDIO_PARAM_ADPCMTYPE *)ComponentParameterStructure;
			if(mOutputAdpcmParam.nPortIndex == pParam->nPortIndex)
			{
//				mOutputAdpcmParam.nBitRate = (mOutputAdpcmParam.nBlockAlign * mOutputAdpcmParam.nSampleRate / mSamplesPerBlock) << 3;
                mOutputAdpcmParam.nExtendDataSize = (OMX_U32)mExtraDataSize;
                memcpy(mOutputAdpcmParam.pExtendData,(OMX_U8 *)mpExtraData,mOutputAdpcmParam.nExtendDataSize);

                memcpy(pParam, &mOutputAdpcmParam, sizeof(OMX_AUDIO_PARAM_ADPCMTYPE));
			}
			else
			{
				err = OMX_ErrorBadPortIndex;
				goto EXIT;
			}
			break;
		}
		default:
		{
			err = MtkOmxAudioEncBase::GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
			break;
		}
	}

EXIT:
	return err;
 }
 OMX_ERRORTYPE MtkOmxAdpcmEnc :: SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
										 OMX_IN OMX_INDEXTYPE nParamIndex,
										 OMX_IN OMX_PTR ComponentParameterStructure)
 {
 	SLOGD("SetParameter +++ !!!");
	OMX_ERRORTYPE err = OMX_ErrorNone;

	if(NULL == ComponentParameterStructure)
	{
		err = OMX_ErrorBadParameter;
		goto EXIT;
	}

	if(mState == OMX_StateInvalid)
	{
		err = OMX_ErrorIncorrectStateOperation;
		goto EXIT;
	}

	switch (nParamIndex)
	{
		case OMX_IndexParamAudioAdpcm:
		{
			SLOGD("SetParameter OMX_IndexParamAudioAdpcm !!!");
			OMX_AUDIO_PARAM_ADPCMTYPE *pParam = (OMX_AUDIO_PARAM_ADPCMTYPE *)ComponentParameterStructure;
			if(mOutputAdpcmParam.nPortIndex == pParam->nPortIndex)
			{
				memcpy(&mOutputAdpcmParam, pParam, sizeof(OMX_AUDIO_PARAM_ADPCMTYPE));
				SLOGD("ADPCM Encode component pParam->nBitsPerSample is %d", pParam->nBitsPerSample);
				if(OMX_ErrorNone != CheckParams(&mOutputAdpcmParam))
				{
					SLOGE("check params failed !!!");
					err = OMX_ErrorBadParameter;
					goto EXIT;
				}
				if (!InitAudioEncoder(NULL))
				{
                	err = OMX_ErrorBadParameter;
            	}
				else
				{

				}
			}
			else
			{
				err = OMX_ErrorBadPortIndex;
				goto EXIT;
			}
			break;
		}
		default:
		{
			err = MtkOmxAudioEncBase::SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
			break;
		}
	}

EXIT:
	return err;
 }

 OMX_ERRORTYPE MtkOmxAdpcmEnc :: CheckParams(OMX_PTR params)
 {
 	SLOGD("Check Params+++");
	OMX_ERRORTYPE err;
	OMX_AUDIO_PARAM_ADPCMTYPE *pParam = (OMX_AUDIO_PARAM_ADPCMTYPE *)params;

	if(pParam->nFormatTag != WAVE_FORMAT_MS_ADPCM && pParam->nFormatTag != WAVE_FORMAT_DVI_IMA_ADPCM)
	{
		SLOGE("Check ADPCM format failed !!!");
		err = OMX_ErrorBadParameter;
		goto EXIT;
	}

	if(pParam->nChannelCount != 1 && pParam->nChannelCount != 2)
	{
		SLOGE("Check ADPCM Channel Count failed !!!");
		err = OMX_ErrorBadParameter;
		goto EXIT;
	}

	if(pParam->nBitsPerSample != 4)
	{
		SLOGE("Check ADPCM BitsPerSample failed !!!");
		err = OMX_ErrorBadParameter;
		goto EXIT;
	}

	err = OMX_ErrorNone;

EXIT:
	return err;
 }

 void MtkOmxAdpcmEnc :: handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
											 OMX_BUFFERHEADERTYPE* pOutputBuf,
											 OMX_BOOL emptyInput, OMX_BOOL fillOutput)
 {
	 SLOGD("pInputBuf size is %d, pOutputBuf size is %d, timestamp of output buffer is %0.4f s, numFramesInOutputBuf is %d",
			 pInputBuf->nFilledLen, pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp / 1E6, numFramesInOutputBuf);
	 SLOGD("pInputBuf is %p, pOutputBuf is %p, pInputBuf->pBuffer is %p, pOutputBuf->pBuffer is %p",
			pInputBuf, pOutputBuf, pInputBuf->pBuffer, pOutputBuf->pBuffer);

	 if(emptyInput == OMX_TRUE)
	 {
		 totalConsumedBytes = 0;
		 HandleEmptyBufferDone(pInputBuf);
	 }
	 else
		 QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));

	 if(fillOutput == OMX_TRUE)
	 {
		 isNewOutputBuffer = OMX_TRUE;
		 numFramesInOutputBuf = 0;
		 HandleFillBufferDone(pOutputBuf);
	 }
	 else
	 {
		 isNewOutputBuffer = OMX_FALSE;
		 QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
	 }
  }

 OMX_BOOL MtkOmxAdpcmEnc :: InitAudioParams()
 {
    SLOGD("InitAudioParams+++");

    strncpy((char*)mCompRole, "audio_encoder.adpcm", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    // init output port format
    mOutputPortFormat.nSize = sizeof(mOutputPortFormat);
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingADPCM;

	// init input port definition
    mInputPortDef.nSize = sizeof(mInputPortDef);
    mInputPortDef.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir = OMX_DirInput;
    mInputPortDef.eDomain = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender = NULL;
    mInputPortDef.format.audio.cMIMEType = (OMX_STRING)"raw";
    mInputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    mInputPortDef.nBufferCountActual = MTK_OMX_NUMBER_INPUT_BUFFER_ADPCM_ENC;
    mInputPortDef.nBufferCountMin = 1;
    mInputPortDef.nBufferSize = MTK_OMX_INPUT_BUFFER_SIZE_ADPCM_ENC;
    mInputPortDef.bEnabled = OMX_TRUE;
    mInputPortDef.bPopulated = OMX_FALSE;

	// init output port definition
    mOutputPortDef.nSize = sizeof(mOutputPortDef);
    mOutputPortDef.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDomain = OMX_PortDomainAudio;
    mOutputPortDef.format.audio.cMIMEType = (OMX_STRING)"audio/x-adpcm-ms";
    mOutputPortDef.format.audio.pNativeRender = 0;
    mOutputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mOutputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingADPCM;
    mOutputPortDef.eDir = OMX_DirOutput;

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_ADPCM_ENC;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_ADPCM_ENC;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

	//Default values for PCM input audio param port
    mInputPcmMode.nSize = sizeof(mInputPcmMode);
    mInputPcmMode.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPcmMode.nChannels = MTK_OMX_OUTPUT_DEFAULT_CHANNEL_COUNT_ADPCM_ENC;
    mInputPcmMode.eNumData = OMX_NumericalDataSigned;
    mInputPcmMode.bInterleaved = OMX_TRUE;
    mInputPcmMode.nBitPerSample = 16;
    mInputPcmMode.nSamplingRate = MTK_OMX_OUTPUT_DEFAULT_SAMPLE_RATE_ADPCM_ENC;
    mInputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mInputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mInputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;


       mOutputAdpcmParam.nSize = sizeof(mOutputAdpcmParam);
	mOutputAdpcmParam.nPortIndex = MTK_OMX_OUTPUT_PORT;
	mOutputAdpcmParam.nFormatTag = WAVE_FORMAT_MS_ADPCM;
	mOutputAdpcmParam.nChannelCount = MTK_OMX_OUTPUT_DEFAULT_CHANNEL_COUNT_ADPCM_ENC;
       mOutputAdpcmParam.nSamplesPerSec = MTK_OMX_OUTPUT_DEFAULT_SAMPLE_RATE_ADPCM_ENC;
	mOutputAdpcmParam.nBlockAlign = 0;
	mOutputAdpcmParam.nBitsPerSample = 4;

	//allocate input/output buffer headers address  array
	mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC((sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);
	if(NULL == mInputBufferHdrs)
	{
		SLOGE("malloc input buffer headers failed!!!");
		return OMX_FALSE;
	}
	MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, (sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);
	mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC((sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);
	if(NULL == mOutputBufferHdrs)
	{
		SLOGE("malloc output buffer headers failed!!!");
		return OMX_FALSE;
	}
	MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, (sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);

	return OMX_TRUE;
 }

 char* MtkOmxAdpcmEnc :: getDumpFilePath()
 {
     struct tm *timeinfo;
     time_t rawtime;
     time(&rawtime);
     timeinfo = localtime(&rawtime);

	 sprintf(mDumpFile, "/sdcard/mtklog/%s", (char *)mCompRole);
	 strftime(mDumpFile + 15 + strlen((char *)mCompRole), 256, "_%Y%m%d%H%M%S.pcm", timeinfo);
	 SLOGD("Dump PCM File is %s", mDumpFile);
	 return mDumpFile;
 }

 // Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
 extern "C" OMX_COMPONENTTYPE* MtkOmxComponentCreate(OMX_STRING componentName)
 {
 	SLOGD("MtkOmxComponentCreate +++, component name is %s", componentName);

	MtkOmxBase* pOmxAdpcmEnc = new MtkOmxAdpcmEnc();
	if(NULL == pOmxAdpcmEnc)
	{
		SLOGE("Create ADPCM component failed !!!");
		return NULL;
	}

	OMX_COMPONENTTYPE* pHandle = pOmxAdpcmEnc->GetComponentHandle();
	if(NULL == pHandle)
	{
		SLOGE("Get ADPCM component handle failed !!!");
		return NULL;
	}

	pOmxAdpcmEnc->ComponentInit(pHandle,componentName);
	return pHandle;

 }
