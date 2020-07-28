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

 #ifndef _MTK_OMX_ADPCM_ENCODE_
 #define _MTK_OMX_ADPCM_ENCODE_

 #include "MtkOmxAudioEncBase.h"
 #include "adpcm_enc_exp.h"

 #define MTK_DEFAULT_DURATION_FOR_ADPCM_BLOCK 20
 #define MTK_MAX_PCM_SIZE_FOR_20MS 960 << 2

 #define MTK_OMX_INPUT_BUFFER_SIZE_ADPCM_ENC  MTK_MAX_PCM_SIZE_FOR_20MS 
 #define MTK_OMX_OUTPUT_BUFFER_SIZE_ADPCM_ENC MTK_MAX_PCM_SIZE_FOR_20MS

   
 #define MTK_OMX_NUMBER_INPUT_BUFFER_ADPCM_ENC  2
 #define MTK_OMX_NUMBER_OUTPUT_BUFFER_ADPCM_ENC 1

 #define MTK_OMX_OUTPUT_DEFAULT_SAMPLE_RATE_ADPCM_ENC    44100
 #define MTK_OMX_OUTPUT_DEFAULT_CHANNEL_COUNT_ADPCM_ENC  2

 typedef struct 
 {
	OMX_U16 bufferSize;
	OMX_U16 dataLength;
	OMX_PTR pBlockPCMBuffer;
 }PendingBuffer;

 class MtkOmxAdpcmEnc : public MtkOmxAudioEncBase
 {
	public:
    	MtkOmxAdpcmEnc();
    	~MtkOmxAdpcmEnc();

		//override these functions from base class for specific OMX Component
		virtual OMX_BOOL InitAudioParams();
		virtual void EncodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf);
		virtual void FlushAudioEncoder();
		virtual void DeinitAudioEncoder();

		virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_INOUT OMX_PTR pCompParam);
		virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_IN OMX_PTR pCompParam);

	private:
		OMX_BOOL InitAudioEncoder(OMX_BUFFERHEADERTYPE* pInputBuf);
		OMX_ERRORTYPE CheckParams(OMX_PTR params);
		char* getDumpFilePath();
		void handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
						OMX_BUFFERHEADERTYPE* pOutputBuf, 
						 OMX_BOOL emptyInput, 
						  OMX_BOOL fillOutput);
		
		ADPCM_ENC_HDL mpHandle;
		OMX_BOOL mEncoderInitFlag;
		OMX_U32 mBitStreamBufferForEncoderSize;
		OMX_U32 mInternalWorkingBufferSize;
		OMX_PTR mpInternalWorkingBuffer;

		OMX_U16 mExtraDataSize;
		OMX_PTR mpExtraData;

		OMX_U16 mSamplesPerBlock;
		PendingBuffer ADPCMPendingBuffer;
		OMX_U16 mBlockAlign;
		OMX_U32 PCMBufferSizeForOneBlock;

		OMX_TICKS mStartTime;
		OMX_U32 mLastSamplesCosumed;

		OMX_BOOL isNewOutputBuffer;
		OMX_U16 maxNumFramesInOutputBuf;
		OMX_U16 numFramesInOutputBuf;

		OMX_U32 totalConsumedBytes;  //just for one input buffer

		//Dump File
    	const char *mDumpProp;
    	char mDumpFile[256];
 };

 #endif
 