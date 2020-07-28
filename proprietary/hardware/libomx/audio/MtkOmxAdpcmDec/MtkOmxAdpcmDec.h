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
 *   MtkOmxAdpcmDec.h
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
 *   
 *
 ****************************************************************************/


#ifndef _MTK_OMX_ADPCM_DEC_
#define _MTK_OMX_ADPCM_DEC_

#include "MtkOmxAudioDecBase.h"
#include "adpcm_dec_exp.h"


#define MTK_OMX_INPUT_BUFFER_SIZE_ADPCM     8192
#define MTK_OMX_OUTPUT_BUFFER_SIZE_ADPCM    MTK_OMX_INPUT_BUFFER_SIZE_ADPCM << 2          

#define MTK_OMX_NUMBER_INPUT_BUFFER_ADPCM   10
#define MTK_OMX_NUMBER_OUTPUT_BUFFER_ADPCM  9

#define MIN_BS_BUFFER_SIZE 512

#define MTK_OMX_AUDIO_DEFAULT_CHANNEL_NUMBERS 2
#define MTK_OMX_AUDIO_DEFAULT_SAMPLINGRATE 44100
#define MTK_OMX_AUDIO_DEFAULT_BITS_PER_SAMPLE 16

#define MTK_OMX_ADPCM_DUMP_DEC_OUTPUT_DATA

class MtkOmxAdpcmDec : public MtkOmxAudioDecBase
{
	public:
		MtkOmxAdpcmDec();
		virtual ~MtkOmxAdpcmDec();
		virtual OMX_BOOL InitAudioParams();
		virtual void DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuffer,OMX_BUFFERHEADERTYPE* pOutputBuffer);
		virtual void FlushAudioDecoder();
		virtual void DeinitAudioDecoder();

		//for update ADPCM paramters
		virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_INOUT OMX_PTR ComponentParameterStructure);
		virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
												OMX_IN OMX_INDEXTYPE nParamIndex,
												OMX_IN OMX_PTR ComponentParameterStructure);

	private:
		OMX_BOOL initAdpcmDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer);
		void handleFormatChanged(OMX_BUFFERHEADERTYPE* pInputBuffer,OMX_BUFFERHEADERTYPE* pOutputBuffer);
		void handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
								OMX_BUFFERHEADERTYPE* pOutputBuf, 
								 OMX_BOOL emptyInput, 
								  OMX_BOOL fillOutput);
		OMX_BOOL mInitFlag;
		OMX_U32 mBsBufSize;
		unsigned int mPcmBufSize;
		
		unsigned int mInterBufSize;
		OMX_PTR mpInterBuf;
		ADPCM_DEC_HDL mpHandle;
		//ADPCM_PARAM *mpAdpcmParam;

		OMX_BOOL mFirstFrameFlag;       // for computing time stamp 
		OMX_U32 mLastSampleCount;

		OMX_TICKS lastTimeStamp;
		OMX_BOOL isNewOutputBuffer;

		OMX_U16 maxNumFramesInOutputBuf;
		OMX_U16 numFramesInOutputBuf;

		OMX_U32 totalConsumedBytes;  //just for one input buffer
        OMX_BOOL fgFlush;
        OMX_BOOL mFormatChange;
};
#endif 



